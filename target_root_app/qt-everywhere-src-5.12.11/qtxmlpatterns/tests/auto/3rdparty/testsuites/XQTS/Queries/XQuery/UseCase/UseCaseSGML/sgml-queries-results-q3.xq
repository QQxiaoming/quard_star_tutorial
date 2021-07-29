(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<result>
  {
    for $c in $input-context//chapter
    where empty($c/intro)
    return $c/section/intro/para
  }
</result> 