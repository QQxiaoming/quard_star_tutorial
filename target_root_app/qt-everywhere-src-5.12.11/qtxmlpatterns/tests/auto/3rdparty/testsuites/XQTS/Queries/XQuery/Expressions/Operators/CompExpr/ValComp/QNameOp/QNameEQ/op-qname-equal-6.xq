(: Name: op-qName-equal-6 :)
(: Description: Evaluation of op-QName-equal operator with two two different qName values (same namespace uri, different local part). Uses the "ne" operator. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:QName("http://www.example.com/example", "person1") ne fn:QName("http://www.example.com/example","person2")