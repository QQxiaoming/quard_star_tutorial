(: Name: op-qName-equal-1 :)
(: Description: Evaluation of op-QName-equal operator with two identical qName values. Uses the "eq" operator. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:QName("http://www.example.com/example", "person") eq fn:QName("http://www.example.com/example", "person")
