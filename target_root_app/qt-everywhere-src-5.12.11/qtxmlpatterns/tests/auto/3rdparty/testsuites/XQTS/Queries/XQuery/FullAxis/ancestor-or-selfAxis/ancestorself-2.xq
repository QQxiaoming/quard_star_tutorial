(: Name: ancestorself-2 :)
(: Description: Evaluation of the ancestor-or-self axis for which the given node does not exists. :)
(: Uses fn:count to avoid empty file. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:count($input-context1/works/employee[1]/ancestor-or-self::noSuchNode)