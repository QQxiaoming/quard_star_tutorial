(: Name: Axes034 :)
(: Description: Path 'descendant-or-self::*' from an element. :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count($input-context//center/descendant-or-self::*)
