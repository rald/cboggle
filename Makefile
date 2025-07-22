boggle: boggle.c trie.c
	gcc boggle.c trie.c -o boggle -I. -L. -g

clean:
	rm boggle.cgi
