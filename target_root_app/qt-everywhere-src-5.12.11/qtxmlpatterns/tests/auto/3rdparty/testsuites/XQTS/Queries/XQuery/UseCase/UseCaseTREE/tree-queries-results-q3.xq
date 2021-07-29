(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<section_count>{ count($input-context//section) }</section_count>, 
<figure_count>{ count($input-context//figure) }</figure_count> 