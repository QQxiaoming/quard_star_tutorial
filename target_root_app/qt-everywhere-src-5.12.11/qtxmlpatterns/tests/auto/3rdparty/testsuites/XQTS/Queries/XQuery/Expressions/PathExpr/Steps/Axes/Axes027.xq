(: Name: Axes027 :)
(: Description: Path 'self::node()' axis from an attribute. :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count($input-context//center/@center-attr-3/self::node())
