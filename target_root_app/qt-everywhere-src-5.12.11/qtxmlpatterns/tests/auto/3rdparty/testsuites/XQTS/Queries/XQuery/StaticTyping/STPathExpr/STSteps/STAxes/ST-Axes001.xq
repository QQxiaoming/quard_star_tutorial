(: Name: ST-Axes001 :)
(: Description: self:: axis with explicit name test called on set of nodes which do not contain any nodes of this name :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count($input-context//center/self::nowhere)
