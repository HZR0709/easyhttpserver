import{_ as x,b as e,o as b,j as g,f as t,e as o,h as a,m as s}from"./index.798bcfdd.js";const w={data(){return{loginForm:{}}},methods:{handleReset(){this.$router.push("/login")}}},V={class:"text-center mt-10 pt-10"},k={class:"mt-6"};function C(r,l,F,R,n,c){const i=e("v-toolbar-title"),_=e("v-toolbar"),d=e("v-text-field"),m=e("v-form"),f=e("v-btn"),u=e("v-card-text"),p=e("v-card"),v=e("v-container");return b(),g(v,{fluid:"",class:"h-100 d-flex justify-center align-center"},{default:t(()=>[o(p,{class:"elevation-10",width:"480",height:"420"},{default:t(()=>[a("div",V,[o(_,{color:"#fff",class:"rounded-t"},{default:t(()=>[o(i,{class:"text-black text-h5 font-weight-semibold pb-6"},{default:t(()=>[s(" \u5FD8\u8BB0\u5BC6\u7801 ")]),_:1})]),_:1}),o(u,null,{default:t(()=>[o(m,{ref:"formRef"},{default:t(()=>[o(d,{color:"blue-lighten-1",outlined:"",label:"Email",placeholder:"Email",variant:"outlined",modelValue:n.loginForm.email,"onUpdate:modelValue":l[0]||(l[0]=h=>n.loginForm.email=h),rules:r.emailRules},null,8,["modelValue","rules"])]),_:1},512),a("div",k,[o(f,{class:"text-capitalize pr-4",style:{color:"white !important"},color:"primary",width:"100%",onClick:c.handleReset},{default:t(()=>[s(" \u91CD\u7F6E\u5BC6\u7801 ")]),_:1},8,["onClick"])])]),_:1})])]),_:1})]),_:1})}const B=x(w,[["render",C]]);export{B as default};