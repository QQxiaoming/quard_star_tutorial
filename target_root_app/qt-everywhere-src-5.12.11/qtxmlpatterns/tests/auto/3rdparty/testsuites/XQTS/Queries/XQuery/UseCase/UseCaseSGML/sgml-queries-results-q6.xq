(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<result>
  {
    for $s in $input-context//section/@shorttitle
    return <stitle>{ $s }</stitle>
  }
</result> 