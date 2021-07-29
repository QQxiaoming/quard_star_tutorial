(:*******************************************************:)
(:Name: Constr-cont-document-5                           :)
(:Written By: Nicolae Brinza                             :)
(:Description: Check the merging of text nodes introduced:)
(:by the contents of a document node. This test case was :)
(:motivated by the resolution of Bug Report #3637        :)
(:*******************************************************:)

count( document {'abc', 'def', document {'ghi', 'jkl'}, 'mno' } /node() )
