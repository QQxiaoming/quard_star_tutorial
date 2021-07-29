(: Name: op-qName-equal-2 :)
(: Description: Evaluation of op-QName-equal operator with two identical qName values. Uses the "ne" operator. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:QName("http://www.example.com/example", "person") ne fn:QName("http://www.example.com/example", "person")