(: Name: fn-id-dtd-18 :)
(: Description: Evaluation of fn:id for which IDREF uses the fn:upper-case function. :)

(: insert-start :)
declare variable $input-context1 as node() external;
(: insert-end :)

fn:id(fn:upper-case("id5"), exactly-one($input-context1/IDS[1]))
