(: Name: fn-root-15:)
(: Description: Evaluation of the fn:root function with argument set to a attribute node by quering an xml file.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:root($input-context1/langs[1]/para[1]/@xml:lang)