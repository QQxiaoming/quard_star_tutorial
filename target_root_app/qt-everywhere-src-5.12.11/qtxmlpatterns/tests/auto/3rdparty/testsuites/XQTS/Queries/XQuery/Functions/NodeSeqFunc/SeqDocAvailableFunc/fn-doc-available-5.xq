(: Name: fn-doc-available-5 :)
(: Description: Evaluation of fn:doc-available function using document URI of a known document.:)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:doc-available(document-uri($input-context))