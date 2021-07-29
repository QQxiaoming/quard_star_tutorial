(:*******************************************************:)
(: Test: K2-followingAxis-1                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Evaluate the 'following' axis on a html document. :)
(:*******************************************************:)
let $i := <html>
    <head/>
    <body>
        <p attr="foo"> </p>
        <p attr="boo"> </p>
        <p> </p>
        <p> </p>
        <p> </p>
    </body>
</html>
return $i//p[1]/following::*