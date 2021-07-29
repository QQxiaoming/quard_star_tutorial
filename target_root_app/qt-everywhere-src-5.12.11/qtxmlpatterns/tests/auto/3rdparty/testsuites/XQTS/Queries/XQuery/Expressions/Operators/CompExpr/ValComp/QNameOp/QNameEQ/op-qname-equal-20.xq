(: Name: op-qName-equal-20 :)
(: Description: Evaluation of op-QName-equal operator with both namespace uri set to the empty string, same local part and no prefix. Uses the "ne" operator. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:QName("", "person") ne fn:QName("","person")