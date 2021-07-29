(: Name: fn-id-dtd-20 :)
(: Description: Evaluation of fn:id for which $arg uses the xs:string function. :)

(: insert-start :)
declare variable $input-context1 as node() external;
(: insert-end :)

fn:id(xs:string("id1"), exactly-one($input-context1/IDS[1]))
