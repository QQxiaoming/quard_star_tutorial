(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<result>
  {
    for $i in $input-context//intro/para[1]
    return
        <first_letter>{ substring(string($i), 1, 1) }</first_letter>
  }
</result> 