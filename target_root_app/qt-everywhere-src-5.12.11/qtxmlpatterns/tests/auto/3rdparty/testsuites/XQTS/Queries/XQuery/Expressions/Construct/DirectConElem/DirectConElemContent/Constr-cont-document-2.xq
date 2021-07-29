(: Name: Constr-cont-document-2 :)
(: Written by: Andrew Eisenberg :)
(: Description: Check the merging of text nodes introduced by the contents of a document node :)
(: This test case was motivated by the resolution of Bug Report 3637 :)

count(<wrapper>  abc {document {'def', <anode/>, 'ghi'}} jkl </wrapper>/node())
