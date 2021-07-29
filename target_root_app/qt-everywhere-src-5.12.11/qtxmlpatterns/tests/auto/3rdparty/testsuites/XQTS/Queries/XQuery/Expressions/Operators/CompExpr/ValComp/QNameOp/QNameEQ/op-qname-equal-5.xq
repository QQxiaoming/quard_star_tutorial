(: Name: op-qName-equal-5 :)
(: Description: Evaluation of op-QName-equal operator with two two different qName values (same namespace uri values, different local part). Uses the "eq" operator. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:QName("http://www.example.com/example", "person1") eq fn:QName("http://www.example.com/example","person2")