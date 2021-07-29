(: Name: Constr-docnode-nested-3 :)
(: Written by: Oliver Hallam :)
(: Date: 2009-10-05 :)
(: Description: Check that text nodes are merged in nested document constructors. :)

count(document {1, document{2}, document { document {()}, document {3}}, 4}/text())
