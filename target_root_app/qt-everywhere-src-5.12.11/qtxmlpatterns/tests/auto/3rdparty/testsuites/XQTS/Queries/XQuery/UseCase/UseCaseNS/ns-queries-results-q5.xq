declare namespace music = "http://www.example.org/music/records";
(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<Q5 xmlns:music="http://www.example.org/music/records">
  {
     $input-context//music:record[music:remark/@xml:lang = "de"]
  }
</Q5> 