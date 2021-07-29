(: Name: Constr-docnode-nested-4 :)
(: Written by: Oliver Hallam :)
(: Date: 2009-10-05 :)
(: Description: Check that empty text nodes are stripped in nested document constructors. :)

count(document {"", document{""}, document { document {()}, document {""}}, ""}/text())
