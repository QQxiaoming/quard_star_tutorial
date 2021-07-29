(: Name: Axes020 :)
(: Description: Path 'self::*' from an element. :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count($input-context//center/self::*)
