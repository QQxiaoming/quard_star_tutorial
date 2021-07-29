(: Name: Axes018 :)
(: Description: Path 'parent::node()' from document element. :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count($input-context/far-north/parent::node())
