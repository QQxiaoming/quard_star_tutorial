(:*******************************************************:)
(: Test: K2-ForExprWithout-8                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Wrap a path expression.                      :)
(:*******************************************************:)
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