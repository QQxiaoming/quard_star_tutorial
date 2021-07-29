(: Name: op-qName-equal-3 :)
(: Description: Evaluation of op-QName-equal operator with two two different qName values (different namespace uri values, same local part). Uses the "eq" operator. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:QName("http://www.example.com/example1", "person") eq fn:QName("http://www.example.com/example2", "person")