(:*******************************************************:)
(:Test: op-add-dayTimeDuration-to-dateTime-10            :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 1, 2005                                     :)
(:Purpose: Evaluates The string value of the "add-dayTimeDuration-to-dateTime" :)
(:operator used together with an "or" expression.        :)
(:*******************************************************:)
 
fn:string((xs:dateTime("1985-07-05T14:14:14Z") + xs:dayTimeDuration("P03DT01H04M"))) or fn:string((xs:dateTime("1985-07-05T15:15:15Z") + xs:dayTimeDuration("P01DT01H03M")))