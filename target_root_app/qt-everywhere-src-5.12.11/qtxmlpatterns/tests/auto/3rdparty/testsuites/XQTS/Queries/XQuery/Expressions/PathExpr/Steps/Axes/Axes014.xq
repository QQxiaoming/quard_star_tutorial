(: Name: Axes014 :)
(: Description: Path 'parent::*' from document element. :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count($input-context/far-north/parent::*)
