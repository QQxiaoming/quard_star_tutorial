(:*******************************************************:)
(: Test: K2-NodeTest-32                                  :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: We evaluate to only atomic values, although the static type does not reflect that. :)
(:*******************************************************:)
<e>
      <a/>
      <b/>
</e>/(if(position() = 10) then (<e/>, .) else 4)