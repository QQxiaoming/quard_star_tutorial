(: Name: Axes031 :)
(: Description: Path 'descendant::*' from an element. :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count($input-context//center/descendant::*)
