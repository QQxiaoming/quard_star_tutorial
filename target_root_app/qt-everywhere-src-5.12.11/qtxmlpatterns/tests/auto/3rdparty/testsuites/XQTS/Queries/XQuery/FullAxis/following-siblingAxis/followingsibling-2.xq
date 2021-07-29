(: Name: followingsibling-2 :)
(: Description: Evaluation of the following-sibling axis for which the given node does not exists. :)
(: Uses fn:count to avoid empty file. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:count($input-context1/works/employee[1]/following-sibling::noSuchNode)