(:*******************************************************:)
(: Test: function-declaration-018.xq          :)
(: Written By: Pulkita Tyagi                             :)
(: Date: Thu Jun  2 00:24:56 2005                        :)
(: Purpose: Demonstrate function declaration in different combination :)
(:********************************************************************:)

declare default element namespace "http://www.example.com/filesystem";
declare function local:udf1 ()
{
  <empty>
   {10*10}
  </empty>
};
local:udf1 ()
