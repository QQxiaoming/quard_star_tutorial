(: Name: XQueryComment002 :)
(: Description: Simple use case for XQuery comments :)

declare namespace fs="http://www.example.com/filesystem";

(: insert-start :)
declare variable $input-context external;
(: insert-end :)


(: This is a comment :)
($input-context//fs:Folder)[1]/fs:File[1]/fs:FileName
