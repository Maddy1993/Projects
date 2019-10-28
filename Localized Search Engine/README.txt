Information Retrieval System:
	This project includeds the methodology to build and compare multiple information systems. Which includes:
		1. BM25.
		2. Smoothed Query Likelihood.
		3. Lucene based search.
		4. TFIDF.
	These are known as retrieval models, which are used to retrieve documents based on their Term Frequencies of the query terms.
	Also, each model modifies the term weighting in a manner to either include or exclude relevance judgements. Though our retrieval
	models are not yet advanced to read the relevance judgments at real-time, we have ignored them during the effectiveness evaluation.

	The Query Expansion is another technique to improve the input to these, in which, we have psuedo-relevance feedback to generate the
	terms which have higher probability to co-occur.

	The user-results are displayed in an HTML page, where the resultant document's content can be viewed in an snippet. The important terms
	or in other words, highly frequent terms are highlighted in the displayed snippets.

	The evaluation of these information retrieval models are performed on basis of four criterias:
		1. Mean Average Precision
		2. Mean Reciprocal Rank
		3. Precision at rank = 5 and rank = 20.

	Their respective precision and recall values are also displayed in the seperate text file for any future evaluation purposes.

Folder Structure: (ordered by their name)

	Corpuses			This folder consists of files which act as the Corpus for the above mentioned 
						information retrieval system.
	Evaluation 			This folder consists of files which are used to evaluate the systems.
		
		Files 			This sub-folder consists of intermediatory files which are used to calculate 
						the effectiveness results of the systems.
		Output			This sub-folder consists of the files which are the result of performing effectiveness
						calculation as mentioned above. Each system is prefixed with their measure.
						
						For example:
							evaluation_results.txt 			consists of the evaluation values of Mean Avergae Precision
															and Mean Reciprocal Rank.
							precision_at_rank_5_20_BM25		reflects the file which consists of precision values at rank = 5 and
															rank = 20 for the all the queries given as input to the BM25.

	Extra_Credit			This folder consists of the code and output files generated for the project's extra-credit task.
		
		Noise-Generator		This folder consists of the python scripts and output files which are the representation of noise-generator
							model.
		Noise-Minimizer		This folder consists of the python scripts and output files which are the representation of the noise-minimzer
							model.

	Indexes				This folder consists of the intermediate files which are used to calculate the resultant scores for a query by any
						search system.

	Retrieval 			This folder is a representation of the core activites of all the information retrieval systems.

		Query_Expansion			This folder represents the python scripts and output files which are used to generate the Query-time stemming
								model.
		
		Retrieval_Model			This folder consists of the core search information and tasks for all the information retrieval systems.

		Stemming_and_Stopping	This folder consists of the files which represent the python scripts used to perform the stemming and stopping
								on the corpus and induce those as the input to the information retrieval systems.

	Snippet_Generation	This folder consists of the pythin scripts which are used to generate snippets and publish the results of a query 
						in an HTML file.
	utilities			This folder represents the intermediate python scripts and queries which are used to provide as an input to the retrieval systems
						tasks.


Execution:
	Third-Party Libraries
		Our project uses the nltk library to perform few key operations within the process.
		So, to execute our project, install nltk library from:
			https://www.nltk.org/install.html

		Our project uses the BeautifulSoup library to perform few parsing tasks. So, to execute our
		project, install BeautifulSoup using the pip tool:
			$ pip install beautifulsoup4

		The Lucene Search has few core JARs which are present in the "utilities" folder. Reference them to
		your Java file before performing a build of the Java program.


	Initially execute the 
		$python corpus_generator.py
		$python inverted_index_generator.py
		$python corpus_statistics.py
		
	In the Retrieval/Retrieval_Model Folder of the project, 
	1. Executing BM25_Information system:
		$python preprocessor.py
		$python search.py
	2. Executing TFIDF Information Systen:
		$python TF_IDF_Retrieval.py
	3. Executing the Smoothed Query Likelihood Model
		$python SQ_Likelihood_Calculator.py
	4. Executing the Lucene Search Engine
		$javac -cp *; LuceneSearchEngine.java
		$java -cp *; LuceneSearchEngine

	To execute the Query_Expansion Model, redirect to Retrieval/Query_Expansion from
	the project root directory,
		$python Task-2.py

	To execute the Snippet_Highlighting, redirect to Snippet_Generation from the project
	root directory
		$python SnippetGeneration_Highlighting.py

	To execute the Stemming and Stopping runs, redirect to Retrieval/Stemming_and_Stopping/
	and execute
		for stemming:
			$python stemmed_corpus_generator.py
			and execute the retrieval models by modifying the commented lines in each core file.
		for stopping:
			$python stopped_corpus_generator.py
			$python stopped_query_generator.py
			and execute the retrieval models by modifying the commented lines in each core file.

	To execute the Evaluation files, redirect to the Evaluation folder from the project root
	and execute
		$python evaluation_script.py

	To execute the Noise Generator Model, redirect to the Extra_Credit/ folder from the root folder
		to generate noise
			execute
				$python noise_generator.py
		to minimize the noise
			execute
				$python noise_minimizer.py

