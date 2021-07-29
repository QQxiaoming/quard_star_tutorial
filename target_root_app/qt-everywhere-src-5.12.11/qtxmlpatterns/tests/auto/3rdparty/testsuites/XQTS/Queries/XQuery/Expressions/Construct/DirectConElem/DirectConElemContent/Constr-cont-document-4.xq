(:*******************************************************:)
(:Name: Constr-cont-document-4                           :)
(:Written By: Nicolae Brinza                             :)
(:Description: Check the merging of text nodes introduced:)
(:by the contents of a document node. This test case was :)
(:motivated by the resolution of Bug Report #3637        :)
(:*******************************************************:)

count( document {'abc', 'def', document {'ghi', <anode/>, 'jkl'}, 'mno' } /node() )
