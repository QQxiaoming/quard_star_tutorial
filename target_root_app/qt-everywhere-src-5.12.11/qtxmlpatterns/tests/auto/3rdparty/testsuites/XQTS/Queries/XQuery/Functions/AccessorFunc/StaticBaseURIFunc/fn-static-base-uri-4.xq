(: Name: fn-static-base-uri-4 :)
(: Description: Evaluation of fn:static-base-uri function using "news". :)
(: Uses fn:string .:)

declare base-uri "news:comp.infosystems.www.servers.unix";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:string(fn:static-base-uri())