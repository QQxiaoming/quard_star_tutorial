(: Name: Axes081 :)
(: Description: Path '//attribute::' with name of attribute, after an element. :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count($input-context//center//attribute::center-attr-2)
