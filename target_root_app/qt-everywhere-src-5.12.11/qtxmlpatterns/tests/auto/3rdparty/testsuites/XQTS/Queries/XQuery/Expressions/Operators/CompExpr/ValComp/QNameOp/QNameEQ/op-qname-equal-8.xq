(: Name: op-qName-equal-8 :)
(: Description: Evaluation of op-QName-equal operator with two same namespace uri, same local part, same prefix. Uses the "ne" operator. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:QName("http://www.example.com/example", "px1:person") ne fn:QName("http://www.example.com/example","px1:person")