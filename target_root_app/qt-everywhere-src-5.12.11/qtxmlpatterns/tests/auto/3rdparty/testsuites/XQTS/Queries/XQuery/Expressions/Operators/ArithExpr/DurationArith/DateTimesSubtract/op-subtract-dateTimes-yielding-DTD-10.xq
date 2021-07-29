(:*******************************************************:)
(:Test: op-subtract-dateTimes-yielding-DTD-10            :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 6, 2005                                     :)
(:Purpose: Evaluates The "subtract-dateTimes-yielding-DTD" operator used  :)
(:together with an "or" expression.                      :)
(:*******************************************************:)
 
fn:string((xs:dateTime("1985-07-05T07:07:07Z") - xs:dateTime("1984-07-05T08:08:08Z"))) or fn:string((xs:dateTime("1985-07-05T09:09:09Z") - xs:dateTime("1984-07-05T10:10:10Z")))