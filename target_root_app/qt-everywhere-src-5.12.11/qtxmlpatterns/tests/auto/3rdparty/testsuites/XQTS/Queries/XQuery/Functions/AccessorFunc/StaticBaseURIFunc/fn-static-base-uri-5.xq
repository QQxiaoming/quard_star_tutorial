(: Name: fn-static-base-uri-5 :)
(: Description: Evaluation of fn:static-base-uri function using "telnet". :)
(: Uses fn:string .:)

declare base-uri "telnet://192.0.2.16:80/";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:string(fn:static-base-uri())