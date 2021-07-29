(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<result>
  {
    let $x := $input-context//xref[@xrefid = "top4"],
        $t := $input-context//title[. << exactly-one($x)]
    return $t[last()]
  }
</result> 
