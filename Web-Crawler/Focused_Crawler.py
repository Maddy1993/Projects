import requests
import re
import time
import enchant
from bs4 import BeautifulSoup

# Variables
# Strings
# start_url = 'https://en.wikipedia.org/wiki/Solar_eclipse'
start_url = ''
appendText = "https://en.wikipedia.org"

# Structures
total_hyper_links = set()
scraped_urls = set()
path = []
depth = {}
keyword_list = []

#CONSTANTS
_MAX_URLS = 1000
_DEPTH = 6

# Settings
dictionary = enchant.Dict("en_US")


def main():

    global start_url
    global keyword_list

    # Input
    start_url = str(input("Please enter the URL to crawl: \n"))
    keywords = str(input("Enter the keywords to search: "))

    # Split the input keywords based on the whitespace character
    keyword_list = keywords.split(" ")

    # With SEED URLs as the node, run DFS algorithm through them
    dfs_search(start_url)

    # Print the length of the URL Set containing the scraped URLs
    print("Total length of URL set: " + str(len(total_hyper_links)))

    # Print the contents of the DFS Search to a file
    print_to_file(total_hyper_links, "focused-bfs-urls.txt")


def process():
    # Set the URL to crawl
    global start_url
    global total_hyper_links
    global scraped_urls
    global dictionary

    # Variables
    curr_hyper_links = []
    scraped_urls = set()

    print("Crawling: " + start_url + "\n")

    # Place a HTTPS request
    # Politeness Policy
    time.sleep(1)
    try:
        pageAddress = requests.get(start_url)
    except:
        print("Invalid URL: " + start_url)
        exit(1)

    # Retrieve all the content
    pageContent = pageAddress.content

    # Create a soup object
    soupObject = BeautifulSoup(pageContent, "html.parser")

    # Finding title of the URL
    title = soupObject.find('title')

    # Store each HTML as plain text
    with open("FOCUSED-BFS_HTML_FILES/%s.html" % title.string, 'wb+') as file:
        file.write(pageContent)

    # Find all the elements of the div tag with the class name: mw-parser-output
    container = soupObject.find(class_='mw-parser-output')

    # Find all the para tags in the main div tag
    para_tags = container.find_all('p')

    for value in para_tags:
        # Find all the anchor tags in each para tag
        anchor_tags = value.find_all('a')

        # Reiterate all the anchor tags to retrieve the hyper links
        for item in anchor_tags:
            if item is not None:
                data = item.contents
                href = item.get("href")
                # scraped_urls.add(href)

                for key in keyword_list:
                    for content in data:
                        if re.search(key, str(content), re.IGNORECASE):
                            pattern = '(.*?)(' + str(key) + ')(.*?)'
                            spell = re.match(pattern, str(content))
                            if spell:
                                if dictionary.check(str(spell.group(1) + spell.group(2) + spell.group(3))):
                                    scraped_urls.add(href)
                    if re.search(key, href, re.IGNORECASE):
                        if href not in scraped_urls:
                            scraped_urls.add(href)

    if scraped_urls.__contains__(None):
        scraped_urls.remove(None)
    # Reiterate all the Hyper Links and filter out the links
    # which do not start with the prefix https://en.wikipedia.org/wiki/
    for value in scraped_urls:
        result = re.match('^.+/wiki/.*$', value)

        if result and (result.group().count(':') == 1) and len(total_hyper_links) < 1000:
            if result.group().count('#') >= 1:
                result = re.match('(.*)#.*', result.group())
                result = result.group(1)
                curr_hyper_links.append(result)
            else:
                curr_hyper_links.append(result.group())

            total_hyper_links.update(curr_hyper_links)

        result = re.match('^/wiki/.*$', value)
        if result and (result.group().count(':') == 0) and len(total_hyper_links) < 1000:
            if result.group().count('#') >= 1:
                result = re.match('(.*)#.*', result.group())
                result = result.group(1)
                curr_hyper_links.append(appendText + result)
            else:
                curr_hyper_links.append(appendText + result.group())

            total_hyper_links.update(curr_hyper_links)

    return curr_hyper_links


# DFS_Loop:
def dfs_search(start):
    # Global Variables
    global start_url
    global total_hyper_links
    global _MAX_URLS
    global _DEPTH
    global depth

    # Local Declarations
    scraped, stack = set(), []
    depth = {"1": [start_url], "2": [], "3": [], "4": [], "5": [], "6": []}
    # stack.append(start)
    scraped.add(start)

    for d in range(1, _DEPTH + 1):
        # Debug Information
        print("Depth: " + str(d) + "/ Crawled URLS: " + str(len(total_hyper_links)) + "\n")

        # While the stack of the depth is not empty and
        # total number of hyoer links did not exceed 1000
        while depth[str(d)] and len(total_hyper_links) < _MAX_URLS:
            stack = depth[str(d)]

            # Debug Information
            print("Current Stack: ")
            print(stack)

            # Until the url_list is not empty
            while stack:
                # Retrieve the last element of the list(stack)
                # print(stack)
                vertex = stack.pop(0)

                # Set the Global URL as the popped value
                start_url = vertex
                # print("in dfs loop: " + start_url)

                # Capture the scraped URLs achieved by the scraping the current
                # URL in context and check if the total number of scraped URLs
                # are less than the required maximum.
                if len(total_hyper_links) < _MAX_URLS and d+1 <= _DEPTH:
                    vertex_url_stack = process()

                    # If the vertex has not been scraped, add it to the scraped list
                    for item in vertex_url_stack:
                        if item not in scraped:
                            # print("item in dfs loop: " + item)
                            # Merge the scraped URLs to the end of the current url_list
                            depth[str(d+1)].append(item)

                            # Add it to the scraped list
                            scraped.add(item)
                else:
                    break


def print_to_file(print_set, print_filename):
    with open(print_filename, 'w+') as file:
        for value in print_set:
            file.write(value + "\n")


if __name__ == "__main__":
    main()