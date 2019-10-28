import requests
import re
import time
from bs4 import BeautifulSoup

# Variables
# Strings
start_url = 'https://en.wikipedia.org/wiki/Solar_eclipse'
appendText = "https://en.wikipedia.org"

# Structures
total_hyper_links = set()
scraped_urls = set()
path = []
depth = {"1": [start_url], "2": [], "3": [], "4": [], "5": [], "6": []}

#CONSTANTS
_MAX_URLS = 1000
_DEPTH = 6


def main():
    # With SEED URLs as the node, run DFS algorithm through them
    dfs_search(start_url)

    # Print the length of the URL Set containing the scraped URLs
    open('dfs-logger.txt', 'a').write("Total length of URL set: " + str(len(total_hyper_links)))

    # Print the contents of the DFS Search to a file
    print_to_file(total_hyper_links, "dfs-urls.txt")

    print("\nCrawl Ended. Check Log file for information")

def process():
    # Set the URL to crawl
    global start_url
    global total_hyper_links
    global scraped_urls

    # Variables
    curr_hyper_links = []
    scraped_urls = set()

    open('dfs-logger.txt', 'a').write("\nCrawling: " + start_url + "\n")

    # Place a HTTPS request
    # Politeness Policy
    time.sleep(1)
    pageAddress = requests.get(start_url)

    # Retrieve all the content
    pageContent = pageAddress.content

    # Create a soup object
    soupObject = BeautifulSoup(pageContent, "html.parser")

    # Finding title of the URL
    title = soupObject.find('title')

    # Store each HTML as plain text
    with open("DFS_HTML_FILES/%s.html" % title.string, 'wb+') as file:
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
                href = item.get("href")
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
                curr_hyper_links.append(appendText+ result)
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
    # stack.append(start)
    scraped.add(start)

    print("Crawl Started.\nCrawling: " + str(start_url))
    for d in range(1, _DEPTH + 1):
        open('dfs-logger.txt', 'a').write("Depth: " + str(d) + "\n")
        while depth[str(d)] and len(total_hyper_links) < _MAX_URLS:
            stack = depth[str(d)]
            open('dfs-logger.txt', 'a').write("Current Stack: ")
            for m in stack:
                open('dfs-logger.txt', 'a').write(m)
            # print(stack)
            # Until the url_list is not empty
            while stack:
                # Retrieve the last element of the list(stack)
                # print(stack)
                vertex = stack.pop()

                # Set the Global URL as the popped value
                start_url = vertex
                # print("in dfs loop: " + start_url)

                # Capture the scraped URLs achieved by the scraping the current
                # URL in context and check if the total number of scraped URLs
                # are less than the required maximum.
                if len(total_hyper_links) < _MAX_URLS:
                    vertex_url_stack = process()

                    # If the vertex has not been scraped, add it to the scraped list
                    for item in vertex_url_stack:
                        if item not in scraped:
                            # print("item in dfs loop: " + item)
                            # Merge the scraped URLs to the end of the current url_list
                            if d+1 <= _DEPTH:
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