(:*******************************************************:)
(: Test: K2-InternalVariablesWithout-13                  :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A prolog containing 20 variable declarations. :)
(:*******************************************************:)
declare variable $local:var1 := 1;
declare variable $local:var2 := 2;
declare variable $local:var3 := 3;
declare variable $local:var4 := 4;
declare variable $local:var5 := 5;
declare variable $local:var6 := 6;
declare variable $local:var7 := 7;
declare variable $local:var8 := 8;
declare variable $local:var9 := 9;
declare variable $local:var10 := 10;
declare variable $local:var11 := 11;
declare variable $local:var12 := 12;
declare variable $local:var13 := 13;
declare variable $local:var14 := 14;
declare variable $local:var15 := 15;
declare variable $local:var16 := 16;
declare variable $local:var17 := 17;
declare variable $local:var18 := 18;
declare variable $local:var19 := 19;
declare variable $local:var20 := 20;
deep-equal((1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20),
           ($local:var1,
            $local:var2,
            $local:var3,
            $local:var4,
            $local:var5,
            $local:var6,
            $local:var7,
            $local:var8,
            $local:var9,
            $local:var10,
            $local:var11,
            $local:var12,
            $local:var13,
            $local:var14,
            $local:var15,
            $local:var16,
            $local:var17,
            $local:var18,
            $local:var19,
            $local:var20))