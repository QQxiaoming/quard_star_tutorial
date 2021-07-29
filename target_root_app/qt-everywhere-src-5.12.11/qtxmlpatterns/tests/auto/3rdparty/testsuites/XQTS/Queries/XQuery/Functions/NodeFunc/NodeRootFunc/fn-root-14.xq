(: Name: fn-root-14:)
(: Description: Evaluation of the fn:root function with argument set to a element node by quering an xml file.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:root(fn:exactly-one($input-context1/langs[1]/para[1]))