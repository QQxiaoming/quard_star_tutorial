(: Name: Axes084 :)
(: Description: Path '//@' with name of attribute, after an element. :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count($input-context//center//@center-attr-2)
