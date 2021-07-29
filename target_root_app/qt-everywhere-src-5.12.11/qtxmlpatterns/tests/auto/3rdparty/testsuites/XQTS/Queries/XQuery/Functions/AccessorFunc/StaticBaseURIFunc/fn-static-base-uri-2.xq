(: Name: fn-static-base-uri-2 :)
(: Description: Evaluation of fn:static-base-uri function using "ftp". :)
(: Uses fn:string .:)

declare base-uri "ftp://ftp.is.co.za/rfc/somefile.txt";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:string(fn:static-base-uri())