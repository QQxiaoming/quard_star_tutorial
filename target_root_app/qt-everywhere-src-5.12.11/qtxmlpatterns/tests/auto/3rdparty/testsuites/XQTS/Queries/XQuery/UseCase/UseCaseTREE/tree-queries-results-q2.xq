(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<figlist>
  {
    for $f in $input-context//figure
    return
        <figure>
            { $f/@* }
            { $f/title }
        </figure>
  }
</figlist> 