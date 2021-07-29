(:*******************************************************:)
(: Test: K2-ForExprWithout-8                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-10-03T14:53:32+01:00                       :)
(: Purpose: Wrap a path expression.                      :)
(:*******************************************************:)
declare construction strip;
(for $d in <people>
    <person  id="person0">
        <name>First</name>
    </person>
    <person>
        <name>Second</name>
    </person>
</people>/person
return
if (some $id in 1
    satisfies typeswitch ($d/@id)
              case $n as node() return $d/@id = "person0"
              default $d return ())
then $d/name
else ())
