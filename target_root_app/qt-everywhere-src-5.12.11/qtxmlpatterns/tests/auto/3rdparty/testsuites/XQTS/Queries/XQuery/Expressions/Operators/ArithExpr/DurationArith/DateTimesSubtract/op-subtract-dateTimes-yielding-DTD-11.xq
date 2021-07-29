(:*******************************************************:)
(:Test: op-subtract-dateTimes-yielding-DTD-11            :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 6, 2005                                     :)
(:Purpose: Evaluates The "subtract-dateTimes-yielding-DTD" operator used  :)
(:as part of a "div" expression.                         :)
(:*******************************************************:)
 
(xs:dateTime("1985-07-05T07:07:07Z") - xs:dateTime("1985-07-05T07:07:07Z"))  div xs:dayTimeDuration("P05DT08H11M")