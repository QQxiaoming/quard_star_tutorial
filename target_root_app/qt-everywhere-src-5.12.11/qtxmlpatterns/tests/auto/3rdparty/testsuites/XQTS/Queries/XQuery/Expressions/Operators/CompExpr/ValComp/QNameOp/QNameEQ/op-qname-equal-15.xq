(: Name: op-qName-equal-15 :)
(: Description: Evaluation of op-QName-equal operator with first namespace uri set to the empty string, same local part and no prefix. Uses the "eq" operator. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:QName("", "person") eq fn:QName("http://www.example.com/example","person")