(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<result>
   {
     $input-context//section[.//title/text()[contains(., "is SGML")]]
   }
</result> 