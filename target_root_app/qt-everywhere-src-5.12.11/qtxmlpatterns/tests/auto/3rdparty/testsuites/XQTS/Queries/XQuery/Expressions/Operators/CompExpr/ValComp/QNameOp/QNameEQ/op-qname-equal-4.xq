(: Name: op-qName-equal-4 :)
(: Description: Evaluation of op-QName-equal operator with two two different qName values. Uses the "ne" operator. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:QName("http://www.example.com/example1", "person") ne fn:QName("http://www.example.com/example2", "person")