(: Name: Axes043 :)
(: Description: Path 'descendant-or-self::node()' from a text node. :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count($input-context//center/text()/descendant-or-self::node())
