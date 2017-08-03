# Vulkan
Vulkanを試してみたために、作ったプロジェクトでした。  
  
このプロジェクトの目的はVulkanを使って「レンダリングパイプライン」の仕組みを構築ためです。  
「フルスクリーンエフェクト」と「シャドウマップ」といったグラフィックスアルゴリズムも実装されました。「VulkanTest」のフォルダの中で、「Core」や
「Program」や「ThirdParty」や「VulkanRenderSystem」などのフォルダがあります。  
*「Core」ー　抽象化グラフィックスAPI。 
*「Program」ー　アプリケーション。
*「ThirdParty」ー　サードパーティ 製のライブラリ
*「VulkanRenderSystem」ー　Vulkanのコード

今、Vulkan を使用したアプリケーションを動作させるですが、もしほかのグラフィックスAPIを使いたいなら、例えばDirectX12、DirectX12 Render Systemを
開発するだけであるいは少しの修正で同じように動作させることができます。  
「フルスクリーンエフェクト」のことですが、このプロジェクトはBloom エフェクトの例として「フルスクリーンエフェクト」の仕組みの使い方をご紹介しま
したが、HDRやDOFなどエフェクトも実現できます。  
   
![Alt text](/image01.png)