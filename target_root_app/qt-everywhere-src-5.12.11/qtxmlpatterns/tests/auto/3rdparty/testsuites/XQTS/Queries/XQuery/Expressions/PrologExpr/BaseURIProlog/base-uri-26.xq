(: Name: base-URI-26 :)
(: Description: Evaluation of base-uri set to a relative value.:)

declare base-uri "abc";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:ends-with(fn:string(fn:static-base-uri()),"abc")