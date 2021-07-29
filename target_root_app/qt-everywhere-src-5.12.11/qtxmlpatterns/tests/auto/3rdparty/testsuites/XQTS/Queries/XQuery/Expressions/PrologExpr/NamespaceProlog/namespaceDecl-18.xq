(: Name: namespaceDecl-18 :)
(: Description: Verify that "ftp://ftp.is.co.za/rfc/somefile.txt" is a vaild namespace declaration :)

declare namespace px = "ftp://ftp.is.co.za/rfc/somefile.txt";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := <px:someElement>some context</px:someElement>
return
 $var