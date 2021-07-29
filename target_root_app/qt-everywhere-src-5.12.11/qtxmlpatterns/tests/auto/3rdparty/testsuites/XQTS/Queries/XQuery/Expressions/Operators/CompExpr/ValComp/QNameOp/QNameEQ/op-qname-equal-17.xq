(: Name: op-qName-equal-17 :)
(: Description: Evaluation of op-QName-equal operator with second namespace uri set to the empty string, same local part and no prefix. Uses the "eq" operator. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:QName("http://www.example.com/example", "person") eq fn:QName("","person")