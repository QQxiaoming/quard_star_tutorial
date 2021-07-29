(: Name: fn-id-dtd-17 :)
(: Description: Evaluation of fn:id for which IDREF uses the fn lower case function. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:id(fn:lower-case("ID1"), $input-context1/IDS[1])