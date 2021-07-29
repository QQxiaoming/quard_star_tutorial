(: FileName: CondExpr016 :)
(: Purpose: if where branches return different valid types :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

($input-context//Folder)[1]/File[ if ( ./@name='File00000000000' ) then 2 else true() ]/FileName
