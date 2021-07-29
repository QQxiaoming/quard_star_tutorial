(: Name: fn-node-name-7 :)
(: Description: Evaluation of node-name function with argument set to a directly constructed element node with namespace declaration.:)
(: Use namespace-uri-from-qName to retrive values from QName. :)
(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:namespace-uri-from-QName(fn:node-name(<anelement xmlns = "http://example.com/examples"></anelement>))