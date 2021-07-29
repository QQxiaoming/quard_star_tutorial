declare namespace music = "http://www.example.org/music/records";
(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<Q2>
  {
    $input-context//music:title
  }
</Q2> 